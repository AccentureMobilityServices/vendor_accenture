//
//  Shader.fsh
//  CubeExample
//
//  Created by Brad Larson on 4/20/2010.
//
varying highp float lightIntensity;
varying highp float zDepth;

void main()
{
	highp vec4 yellow = vec4(1.0 - zDepth, zDepth, zDepth, 1.0);
	gl_FragColor = vec4((yellow * (lightIntensity) ).rgb, 1.0);
	
	//vec4 v = vec4(zDepth, zDepth, 0.7, 1.0); 
	//gl_FragColor = v;
}
