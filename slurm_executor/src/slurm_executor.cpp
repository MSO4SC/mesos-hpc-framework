// Copyright 2017 MSO4SC - javier.carnero@atos.net
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <queue>
#include <string>

#include <mesos/http.hpp>

#include <mesos/v1/executor.hpp>
#include <mesos/v1/mesos.hpp>

#include <process/defer.hpp>
#include <process/delay.hpp>
#include <process/owned.hpp>
#include <process/process.hpp>

#include <stout/exit.hpp>
#include <stout/linkedhashmap.hpp>
#include <stout/option.hpp>
#include <stout/os.hpp>
#include <stout/uuid.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/random/random_device.hpp>

#include <libssh/libssh.h>

#include "jobsettings.pb.h"

using std::cout;
using std::cerr;
using std::endl;
using std::queue;
using std::string;
using std::stringstream;
using std::istringstream;
using std::list;

using mesos::v1::ExecutorID;
using mesos::v1::FrameworkID;
using mesos::v1::TaskID;
using mesos::v1::TaskInfo;
using mesos::v1::TaskState;
using mesos::v1::TaskStatus;

using mesos::v1::executor::Call;
using mesos::v1::executor::Event;
using mesos::v1::executor::Mesos;

using process::spawn;
using process::wait;

using boost::thread;

using boost::property_tree::ptree;
using boost::property_tree::read_json;

class SlurmExecutor : public process::Process<SlurmExecutor> {
 public:
  SlurmExecutor(const FrameworkID& _frameworkId, const ExecutorID& _executorId)
      : frameworkId(_frameworkId),
        executorId(_executorId),
        state(DISCONNECTED),
        slurm_control_th(0),
        my_ssh_session(0) {
  }

  void connected() {
    state = CONNECTED;

    doReliableRegistration();
  }

  void doReliableRegistration() {
    if (state == SUBSCRIBED || state == DISCONNECTED) {
      return;
    }

    Call call;
    call.mutable_framework_id()->CopyFrom(frameworkId);
    call.mutable_executor_id()->CopyFrom(executorId);

    call.set_type(Call::SUBSCRIBE);

    Call::Subscribe* subscribe = call.mutable_subscribe();

    // Send all unacknowledged updates.
    for (const Call::Update& update : updates.values()) {
      subscribe->add_unacknowledged_updates()->MergeFrom(update);
    }

    // Send all unacknowledged tasks.
    for (const TaskInfo& task : tasks.values()) {
      subscribe->add_unacknowledged_tasks()->MergeFrom(task);
    }

    mesos->send(call);

    process::delay(Seconds(1), self(), &Self::doReliableRegistration);
  }

  void disconnected() {
    state = DISCONNECTED;

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
  }

  void sendStatusUpdate(const TaskInfo& task, const TaskState& state) {
    UUID uuid = UUID::random();

    TaskStatus status;
    status.mutable_task_id()->CopyFrom(task.task_id());
    status.mutable_executor_id()->CopyFrom(executorId);
    status.set_state(state);
    status.set_source(TaskStatus::SOURCE_EXECUTOR);
    status.set_uuid(uuid.toBytes());

    Call call;
    call.mutable_framework_id()->CopyFrom(frameworkId);
    call.mutable_executor_id()->CopyFrom(executorId);

    call.set_type(Call::UPDATE);

    call.mutable_update()->mutable_status()->CopyFrom(status);

    // Capture the status update.
    updates[uuid] = call.update();

    mesos->send(call);
  }

  void received(queue<Event> events) {
    while (!events.empty()) {
      Event event = events.front();
      events.pop();

      switch (event.type()) {
        case Event::SUBSCRIBED: {
          cout << "Received a SUBSCRIBED event" << endl;

          state = SUBSCRIBED;
          break;
        }

        case Event::LAUNCH: {
          const TaskInfo& task = event.launch().task();
          tasks[task.task_id()] = task;  // Save task copy to be acknowledge.

          cout << "Starting task " << task.task_id().value() << endl;

          // Do the slurm call
          slurm_framework::jobsettings job_settings;
          string data = task.data();
          if (job_settings.ParseFromString(data)) {
            string name = task.name() + getRandomString(8);
            string call = getSlurmCall(name, job_settings);
            cout << "DEBUG: Slurm call \"" << call << "\"" << endl;

            if (callSlurm(call) == SSH_OK) {
              sendStatusUpdate(task, TaskState::TASK_STARTING);
              cout << "DEBUG: slurm call ok" << endl;

              ulong jobid;
              if (getJobIdByName(name, jobid) == SSH_OK) {
                cout << "DEBUG: jobid = " << jobid << endl;
                slurm_tasks.emplace_back(task);  // Save task copy to monitor it in Slurm
              } else {
                sendStatusUpdate(task, TaskState::TASK_FAILED);
              }
            } else {
              sendStatusUpdate(task, TaskState::TASK_FAILED);
            }
          } else {
            sendStatusUpdate(task, TaskState::TASK_FAILED);
          }

          break;
        }

        case Event::LAUNCH_GROUP: {
          cout << "Received a LAUNCH_GROUP event";
          // TODO Implement this.
          break;
        }

        case Event::KILL: {
          cout << "Received a KILL event" << endl;
          break;
        }

        case Event::ACKNOWLEDGED: {
          cout << "Received an ACKNOWLEDGED event" << endl;

          // Remove the corresponding update.
          updates.erase(UUID::fromBytes(event.acknowledged().uuid()).get());

          // Remove the corresponding task.
          tasks.erase(event.acknowledged().task_id());
          break;
        }

        case Event::MESSAGE: {
          cout << "Received a MESSAGE event" << endl;
          break;
        }

        case Event::SHUTDOWN: {
          cout << "Received a SHUTDOWN event" << endl;
          break;
        }

        case Event::ERROR: {
          cout << "Received an ERROR event" << endl;
          break;
        }

        case Event::UNKNOWN: {
          LOG(WARNING)<< "Received an UNKNOWN event and ignored";
          break;
        }
      }
    }
  }

  void slurmControlLoop() {
    while (true) {
      boost::this_thread::sleep(boost::posix_time::seconds(1));

      tasks_mutex.lock();

      for (const TaskInfo& task : slurm_tasks) {

        //TODO check task status in Slurm and send update if it is necessary

        cout << "Starting task " << task.task_id().value() << endl;

        sendStatusUpdate(task, TaskState::TASK_RUNNING);

        cout << "Finishing task " << task.task_id().value() << endl;

        sendStatusUpdate(task, TaskState::TASK_FINISHED);
      }
      slurm_tasks.clear();

      tasks_mutex.unlock();
    }
  }

protected:
  virtual void initialize() {
    // We initialize the library here to ensure that callbacks are only invoked
    // after the process has spawned.
    mesos.reset(new Mesos(
            mesos::ContentType::PROTOBUF,
            process::defer(self(), &Self::connected),
            process::defer(self(), &Self::disconnected),
            process::defer(self(), &Self::received, lambda::_1)));

    //get ssh parameters
    Option<string> host = os::getenv("SSH_HOST");
    if (host.isNone()) {
      EXIT(EXIT_FAILURE) << "Expecting ssh host in the environment";
    }
    Option<string> user = os::getenv("SSH_USER");
    if (user.isNone()) {
      EXIT(EXIT_FAILURE) << "Expecting ssh user in the environment";
    }
    Option<string> password = os::getenv("SSH_PASS");
    if (password.isNone()) {
      EXIT(EXIT_FAILURE) << "Expecting ssh password in the environment";
    }

    // Open session and set options
    int verbosity = SSH_LOG_PROTOCOL;
    int port = 22;

    my_ssh_session = ssh_new();
    if (my_ssh_session == NULL) exit(-1);

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, host.get().c_str());
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, user.get().c_str());
    ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);

    // Connect to server
    int rc = ssh_connect(my_ssh_session);
    if (rc != SSH_OK) {
      fprintf(stderr, "Error connecting to %s: %s\n",
          host, ssh_get_error(my_ssh_session));
      exit(-1);
    }

    // Verify the server's identity
    if (verifyKnownhost() < 0) {
      ssh_disconnect(my_ssh_session);
      ssh_free(my_ssh_session);
      exit(-1);
    }

    // Authenticate ourselves TODO public key authentication (see http://api.libssh.org/master/libssh_tutor_authentication.html#authentication_details)
    rc = ssh_userauth_password(my_ssh_session, NULL, password.get().c_str());
    if (rc != SSH_AUTH_SUCCESS) {
      fprintf(stderr, "Error authenticating with password: %s\n",
          ssh_get_error(my_ssh_session));
      ssh_disconnect(my_ssh_session);
      ssh_free(my_ssh_session);
      exit(-1);
    }

    // Create and start Slurm control thread
    slurm_control_th = new boost::thread(&SlurmExecutor::slurmControlLoop, this);
  }

private:
  const FrameworkID frameworkId;
  const ExecutorID executorId;
  process::Owned<Mesos> mesos;
  enum State
  {
    CONNECTED,
    DISCONNECTED,
    SUBSCRIBED
  }state;

  LinkedHashMap<UUID, Call::Update> updates;  // Unacknowledged updates.
  LinkedHashMap<TaskID, TaskInfo> tasks;// Unacknowledged tasks.

  list<TaskInfo> slurm_tasks;// Tasks currently managed by Slurm.
  boost::mutex tasks_mutex;// Mutex to lock the slurm_tasks list between threads.
  thread * slurm_control_th;// Slurm control thread.

  ssh_session my_ssh_session;

  const string alphanum = string(
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz");

  int verifyKnownhost()
  {
    int state, hlen;
    unsigned char *hash = NULL;
    char *hexa;
    char buf[10];
    state = ssh_is_server_known(my_ssh_session);
    hlen = ssh_get_pubkey_hash(my_ssh_session, &hash);
    if (hlen < 0)
    return -1;
    switch (state)
    {
      case SSH_SERVER_KNOWN_OK:
      break; /* ok */
      case SSH_SERVER_KNOWN_CHANGED:
      fprintf(stderr, "Host key for server changed: it is now:\n");
      ssh_print_hexa("Public key hash", hash, hlen);
      fprintf(stderr, "For security reasons, connection will be stopped\n");
      free(hash);
      return -1;
      case SSH_SERVER_FOUND_OTHER:
      fprintf(stderr, "The host key for this server was not found but an other"
          "type of key exists.\n");
      fprintf(stderr, "An attacker might change the default server key to"
          "confuse your client into thinking the key does not exist\n");
      free(hash);
      return -1;
      case SSH_SERVER_FILE_NOT_FOUND:
      fprintf(stderr, "Could not find known host file.\n");
      fprintf(stderr, "If you accept the host key here, the file will be"
          "automatically created.\n");
      /* fallback to SSH_SERVER_NOT_KNOWN behavior */
      case SSH_SERVER_NOT_KNOWN:
      hexa = ssh_get_hexa(hash, hlen);
      fprintf(stderr,"The server is unknown. Do you trust the host key?\n");
      fprintf(stderr, "Public key hash: %s\n", hexa);
      free(hexa);
      if (fgets(buf, sizeof(buf), stdin) == NULL)  //FIXME what to do in this case?
      {
        free(hash);
        return -1;
      }
      if (strncasecmp(buf, "yes", 3) != 0)
      {
        free(hash);
        return -1;
      }
      if (ssh_write_knownhost(my_ssh_session) < 0)
      {
        fprintf(stderr, "Error %s\n", strerror(errno));
        free(hash);
        return -1;
      }
      break;
      case SSH_SERVER_ERROR:
      fprintf(stderr, "Error %s", ssh_get_error(my_ssh_session));
      free(hash);
      return -1;
    }
    free(hash);
    return 0;
  }

  string getSlurmCall(const string& job_name, const slurm_framework::jobsettings& job_settings) const {
    stringstream slurm_call_stream;

    //slurm command (srun on sbatch) plus nohup to detach the execution from this session
    slurm_call_stream << "nohup " << job_settings.scommand() << " -J '" << job_name << "'";

    //add slurm parameters
    if (job_settings.has_partition()) {
      slurm_call_stream << " -p " << job_settings.partition();
    }

    if (job_settings.has_nodes()) {
      slurm_call_stream << " -N " << job_settings.nodes();
    }

    if (job_settings.has_tasks()) {
      slurm_call_stream << " -n " << job_settings.tasks();
    }

    if (job_settings.has_tasks_per_node()) {
      slurm_call_stream << " --ntasks-per-node=" << job_settings.tasks_per_node();
    }

    if (job_settings.has_max_time()) {
      slurm_call_stream << " -t " << job_settings.max_time();
    }

    //add executable and arguments
    slurm_call_stream << " " << job_settings.command();

    //disable output
    slurm_call_stream << " >/dev/null 2>&1";

    //run in the background (don't get blocked until it finish)
    slurm_call_stream << " &";

    return slurm_call_stream.str();
  }

  int callSlurm(const string& slurm_call) const {
    ssh_channel channel;
    int rc;
    char buffer[256];
    int nbytes;
    channel = ssh_channel_new(my_ssh_session);
    if (channel == NULL)
    return SSH_ERROR;
    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
      ssh_channel_free(channel);
      return rc;
    }
    rc = ssh_channel_request_exec(channel, slurm_call.c_str());
    if (rc != SSH_OK) {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return rc;
    }
//    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
//    while (nbytes > 0)
//    {
//      if (write(1, buffer, nbytes) != (unsigned int) nbytes)
//      {
//        ssh_channel_close(channel);
//        ssh_channel_free(channel);
//        return SSH_ERROR;
//      }
//      nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
//    }
//
//    if (nbytes < 0)
//    {
//      ssh_channel_close(channel);
//      ssh_channel_free(channel);
//      return SSH_ERROR;
//    }
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return SSH_OK;
  }

  int getJobIdByName(const string& name, ulong& jobid) const {
    //set sacct command
    string command = "sacct -n -o jobid --name='" + name +"'";
    cout << "DEBUG: jobid slurm comand: " << command << endl;

    ssh_channel channel;
    int rc;
    char buffer[256];
    int nbytes;
    channel = ssh_channel_new(my_ssh_session);
    if (channel == NULL)
    return SSH_ERROR;
    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
      ssh_channel_free(channel);
      return rc;
    }
    rc = ssh_channel_request_exec(channel, command.c_str());
    if (rc != SSH_OK) {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return rc;
    }

    stringstream output;
    nbytes = ssh_channel_read_timeout(channel, buffer, sizeof(buffer), 0, 5000);
    while (nbytes > 0) {
      cout << "DEBUG: Leyendo datos!!" << endl;
      output << buffer;
      nbytes = ssh_channel_read_timeout(channel, buffer, sizeof(buffer), 0, 5000);
    }
    cout << "DEBUG: Fin datos leidos!!" << endl;

    if (nbytes < 0) {
      cout << "DEBUG: nbytes < 0!!: " << endl;
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return SSH_ERROR;
    }

    cout << "DEBUG: Received **" << output.str() << "**" << endl;
    jobid = std::stoul(output.str());

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return SSH_OK;
  }

  string getRandomString(const int len) {
    stringstream ramdom_ss;

    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(0, alphanum.size() - 1);
    for(int i = 0; i < len; ++i) {
        ramdom_ss << alphanum[index_dist(rng)];
    }

    return ramdom_ss.str();
  }
};

int main() {
  std::ofstream out("/home/vagrant/out.txt");
  std::ofstream err("/home/vagrant/err.txt");
  cout.rdbuf(out.rdbuf());  //redirect std::cout to out.txt
  cerr.rdbuf(err.rdbuf());

  FrameworkID frameworkId;
  ExecutorID executorId;

  Option<string> value;

  value = os::getenv("MESOS_FRAMEWORK_ID");
  if (value.isNone()) {
    EXIT(EXIT_FAILURE)
        << "Expecting 'MESOS_FRAMEWORK_ID' to be set in the environment";
  }
  frameworkId.set_value(value.get());

  value = os::getenv("MESOS_EXECUTOR_ID");
  if (value.isNone()) {
    EXIT(EXIT_FAILURE)
        << "Expecting 'MESOS_EXECUTOR_ID' to be set in the environment";
  }
  executorId.set_value(value.get());

  process::Owned<SlurmExecutor> executor(
      new SlurmExecutor(frameworkId, executorId));

  process::spawn(executor.get());
  process::wait(executor.get());

  return EXIT_SUCCESS;
}