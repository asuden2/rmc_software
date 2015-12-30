#include "state_machine.h"

StateMachineBase::StateMachineBase(void):
	_moveBaseAC("move_base", true), _panServoAC("pan_servo", true), _nhLocal("~"),
   MAX_GOAL_POINTS(12), _robotState(eInitializing)
{

}

StateMachineBase::~StateMachineBase(void)
{

}

bool StateMachineBase::Initialize()
{
  std::string   goalParamName = "GoalPoint";

  for(int i = 0; i < MAX_GOAL_POINTS; i++)
  {
    std::ostringstream gp; gp << "GoalPoint" << i;

    ROS_DEBUG_STREAM("Loaded param: " << gp.str());

    if (_nhLocal.hasParam(gp.str()))
    {
      std::vector<double>   goalXY;

      geometry_msgs::Pose   tmpPose;

      _nhLocal.getParam(gp.str(), goalXY);

      tmpPose.position.x  = goalXY[0];
      tmpPose.position.y  = goalXY[1];
      //tf::quaternionTFToMsg(tf::createIdentityQuaternion(), tmpPose.orientation);
      //tmpPose.orientation = tf::createQuaternionMsgYaw(3.14159);

      _goalPointsQueue.push(tmpPose);
    }
    else
    {
      ROS_ERROR_STREAM("Please fill the goalpoints.yaml for GoalPoint" << i);
      return false;
    }
  }

//  _servoSub = _nh.subscribe<std_msgs::Bool> ("servo_camera_state", 1, &StateMachineBase::servoCameraState, this);

  return true;
}

void StateMachineBase::servoCameraState(const std_msgs::Bool::ConstPtr& servoStateOK)
{
	// if servoState = true meaning the aruco marker is found
	if( servoStateOK->data )
	{
				
	}
	else // marker is not found
	{
		_stateStack.push(_robotState); 

		// Stack current goal, drive backward 0.5 meters, if we stil dont get it,
		// then just rely on imu		
		if( _robotState	== StateMachineBase::eRelocalize)
		{
							
		}
	}		
}

void StateMachineBase::moveToGoalPoint()
{
	
}

void StateMachineBase::run()
{
  /*
	while(!_moveBaseAC.waitForServer(ros::Duration(2.0)))
	{
    ROS_INFO("Waiting for the move_base action server...");
	}
	ROS_INFO("Established Connection with move_base ActionServer!");
  */
  while(!_panServoAC.waitForServer(ros::Duration(5.0)))
  {
    ROS_INFO("Waiting for the pan_servo action server...");
  }
  ROS_INFO("Established Connection with pan_servo ActionServer!");

  rmc_simulation::PanServoGoal goalRequest;
  _panServoAC.sendGoal(goalRequest);

  ROS_INFO("Sent PanServoGoal, waiting for result...");
  _panServoAC.waitForResult();
  ROS_INFO("Got result...");

  rmc_simulation::PanServoResult::ConstPtr goalResult;
  goalResult = _panServoAC.getResult();

  if(goalResult)
    ROS_INFO("completed_panning = true");
  else
    ROS_INFO("completed_panning = false");

  /*
	while(!_goalPointsQueue.empty())
	{
    move_base_msgs::MoveBaseGoal moveBaseGoal;
	
		moveBaseGoal.target_pose.header.frame_id   = "map";
		moveBaseGoal.target_pose.header.stamp      = ros::Time::now();

		moveBaseGoal.target_pose.pose.position     = _goalPointsQueue.front().position;
		moveBaseGoal.target_pose.pose.orientation  = tf::createQuaternionMsgFromRollPitchYaw(0, 0, 0);

		ROS_INFO_STREAM("Sending goal(X, Y):" << "[ " << moveBaseGoal.target_pose.pose.position.x << " , "
                                              		<< moveBaseGoal.target_pose.pose.position.y << " ]");

		_moveBaseAC.sendGoal(moveBaseGoal);

    ROS_INFO("Sent Goal...");

		_moveBaseAC.waitForResult();

    ROS_INFO("Got result...");

		if(_moveBaseAC.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
		{
	      ROS_INFO("Removing goal from queue...");
  			_goalPointsQueue.pop();
  			ROS_INFO("Succesfully moved to GoalPoint.");
		}
		else
		{
  			ROS_WARN("Failed to move to GoalPoint.");
		}
	}
  */
}
