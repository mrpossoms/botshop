#include <stdio.h>
#include <getopt.h>

#include "core.h"
#include "body.hpp"
#include "form.hpp"
#include "camera.hpp"
#include "world.hpp"
#include "material.hpp"
#include "renderergl.hpp"
#include "cli.h"
#include "ctx.h"

botshop::Context CTX;

int main(int argc, char* argv[])
{

	srand(time(NULL));

	botshop::RendererGL* renderer = new botshop::RendererGL("data/");

	// int fd = open("./untitled.obj", O_RDONLY);
	// printf("%d\n", errno);
	// botshop::OBJModel mod(fd);

	botshop::World world;

	botshop::Model* car_model = botshop::ModelFactory::get_model("data/car_body.obj");
	botshop::Model* wheel_model = botshop::ModelFactory::get_model("data/wheel.obj");
	// botshop::Model* box_model = botshop::ModelFactory::get_model("data/sphereized_cube.obj");
	botshop::Model* box_model = botshop::ModelFactory::get_model("data/cube.obj");

	botshop::Form car_body(world, car_model);
	botshop::Form box0(world, box_model);
	botshop::Form box1(world, box_model);
	botshop::Form box2(world, box_model);
	botshop::Form wheel0(world, wheel_model);
	botshop::Form wheel1(world, wheel_model);
	botshop::Form wheel2(world, wheel_model);
	botshop::Form wheel3(world, wheel_model);


	botshop::Camera cam(world, M_PI / 4, 160, 120);

	// GLuint framebuffer = botshop::MaterialFactory::create_framebuffer(256, 256);
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Vec3 cd = car_model->box_dimensions();
	printf("%f %f %f\n", cd.x, cd.y, cd.z);

	// car_body.is_a_mesh(car_model)->position(0, 0, 9);
	car_body.is_a_box(cd.x, cd.y, cd.z)->position(0, 0, 1);
	wheel0.is_a_sphere(0.082)->position(0.15, -0.15, 1);
	wheel1.is_a_sphere(0.082)->position(-0.15, -0.15, 1);
	wheel2.is_a_sphere(0.082)->position(-0.15, 0.15, 1);
	wheel3.is_a_sphere(0.082)->position(0.15, 0.15, 1);

	std::vector<botshop::Joint*> joints;

	joints.push_back(botshop::Joint::wheel(wheel0, Vec3(0, 0, 1), Vec3(1, 0, 0)));
	joints.push_back(botshop::Joint::wheel(wheel1, Vec3(0, 0, 1), Vec3(1, 0, 0)));
	joints.push_back(botshop::Joint::wheel(wheel2, Vec3(0, 0, 1), Vec3(1, 0, 0)));
	joints.push_back(botshop::Joint::wheel(wheel3, Vec3(0, 0, 1), Vec3(1, 0, 0)));

	for(auto joint : joints)
	{
		car_body.attach(joint);
	}

	box0.is_a_box(2, 2, 2)->position(0, 2, 5);
	box1.is_a_box(2, 2, 2)->position(-3, 0, 2);
	box2.is_a_box(2, 2, 2)->position(3, 0, 2);

	cam.is_a_sphere(0.05)->position(0, 0, 10);

	world += car_body;
	world += wheel0;
	world += wheel1;
	world += wheel2;
	world += wheel3;
	// world += box0;
	world += box1;
	world += box2;
	world += cam;

 // 	cam.torque(1, 0, 1);

 	car_body.torque(botshop::randf(20) - 10, botshop::randf(20) - 10, botshop::randf(20) - 10);

	box0.torque(botshop::randf(20) - 10, botshop::randf(20) - 10, botshop::randf(20) - 10);
	box1.torque(botshop::randf(20) - 10, botshop::randf(20) - 10, botshop::randf(20) - 10);
	box2.torque(botshop::randf(20) - 10, botshop::randf(20) - 10, botshop::randf(20) - 10);

	time_t now = time(NULL);
	int frames = 0;

	while(!renderer->should_close())
	{
		const float s = 10;
		float steer = 0;

		if(glfwGetKey(renderer->win, GLFW_KEY_UP) == GLFW_PRESS)
		{
			// s = 1;
			wheel0.torque(-s, 0, 0);
			wheel1.torque(-s, 0, 0);
		}

		if(glfwGetKey(renderer->win, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			// s = -1;
			wheel0.torque(s, 0, 0);
			wheel1.torque(s, 0, 0);
		}

		if(glfwGetKey(renderer->win, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			steer = -1;
		}

		if(glfwGetKey(renderer->win, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			steer = 1;
		}

		for(int i = 2; i--;)
		{
			auto joint = joints[i + 2];
			dReal v = dJointGetHinge2Angle1 (joint->ode_joint);
			dJointSetHinge2Angle1(joint->ode_joint, v);
			// if (v > 0.1) v = 0.1;
			// if (v < -0.1) v = -0.1;
			// v *= 10.0;
			// dJointSetHinge2Param (joint->ode_joint,dParamVel,v);
			// dJointSetHinge2Param (joint->ode_joint,dParamFMax,0.2);
			// dJointSetHinge2Param (joint->ode_joint,dParamLoStop,-0.75);
			// dJointSetHinge2Param (joint->ode_joint,dParamHiStop,0.75);
			// dJointSetHinge2Param (joint->ode_joint,dParamFudgeFactor,0.1);
		}


		// dJointSetHinge2Param (rear_wheel_axle0->ode_joint,dParamVel2, s);
	    // dJointSetHinge2Param (rear_wheel_axle0->ode_joint,dParamFMax2,0.1);
		// dJointSetHinge2Param (rear_wheel_axle1->ode_joint,dParamVel2, s);
	    // dJointSetHinge2Param (rear_wheel_axle1->ode_joint,dParamFMax2,0.1);
		Vec3 cam_pos = cam.position();

		world.step(0.05);
		cam.position(cam_pos);

		box1.position(-3, 0, 2);

		renderer->draw(&cam, &world);

		if(now != time(NULL))
		{
			now = time(NULL);
			std::cerr << frames << "fps\n";
			frames = 0;
		}

		++frames;
	}

	return 0;
}
