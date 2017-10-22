//
//  Shader.vsh
//  OPjectiveExample
//
//  Created by Kirk Roerig on 8/20/14.
//  Copyright (c) 2014 OPifex. All rights reserved.
//

attribute vec3 position;
attribute vec3 normal;

uniform mat4 view_projection;
uniform mat4 world;

void main()
{
	//gl_Position = view_projection * world * vec4(position, 1);
	vec4 pos = vec4(position, 1.0);
	pos.z = 0.0;
	gl_Position = pos;
}
