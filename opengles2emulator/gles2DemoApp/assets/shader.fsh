//
//  Shader.fsh
//  CubeExample
//
//  Created by Brad Larson on 4/20/2010.
//
varying highp float lightIntensity;
varying highp float zDepth;
varying highp vec2 tc;
uniform sampler2D sTexture;

void main()
{
	gl_FragColor = texture2D(sTexture, tc)*lightIntensity;
}
