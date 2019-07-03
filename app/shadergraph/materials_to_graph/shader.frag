#pragma include <surface.frag>

uniform sampler2D input_property0;
uniform sampler2D input_property1;
uniform float input_property3;
uniform sampler2D input_property2;
vec3 temp9;
vec3 temp3;
vec4 temp5;
vec3 temp6;
float temp0;
vec4 temp8;
vec4 temp2;
void surface(inout Material material){
// Texture Property - e124bdb7-c4cf-44dc-939c-7f271e13bbad
temp5 = texture(input_property0,v_texCoord);
temp6 = temp5.xyz * vec3(2) - vec3(1);

// Texture Property - ee53ee6c-7586-481e-8515-2ef2966dbbf8
temp8 = texture(input_property1,v_texCoord);
temp9 = temp8.xyz * vec3(2) - vec3(1);

// Texture Property - bfdf5d36-6478-4cda-90c5-6ee02fa2027c
temp2 = texture(input_property2,v_texCoord);
temp3 = temp2.xyz * vec3(2) - vec3(1);

// Surface Material - 7cf4d9d3-5d33-4168-9251-a92f73bf9e49
material.diffuse = temp5.xyz;
material.specular = temp8.xyz;
material.shininess = input_property3;
material.normal = temp3;
material.ambient = vec3(0.0f, 0.0f, 0.0f);
material.emission = vec3(0.0f, 0.0f, 0.0f);
material.alpha = 1.0f;
material.alphaCutoff = 0.0f;


}
