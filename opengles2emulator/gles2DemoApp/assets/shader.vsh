//
//  Shader.vsh
//  CubeExample
//
//  Created by Brad Larson on 4/20/2010.
//

attribute vec4 position;
attribute vec4 normal;

varying float lightIntensity;
varying float zDepth;

uniform mat4 modelViewProjMatrix;
uniform vec3 lightDirection;
void main()
{
//    gl_Position = position;
	vec4 newNormal = modelViewProjMatrix * normal;
//	newNormal = normalize(newNormal);
	vec4 newPosition = modelViewProjMatrix * position;
    gl_Position = newPosition;

	lightIntensity = max(0.0, dot(newNormal.xyz, lightDirection));
	zDepth = (2.0 - (1.0 + position.xyz.z))/2.0;
}
