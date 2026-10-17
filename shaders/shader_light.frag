#version 330

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 vColor;

out vec4 color;

//tienen que ser iguales a los de src/CommonValues.h
const int MAX_POINT_LIGHTS = 3;
const int MAX_SPOT_LIGHTS = 4;

struct Light
{
	vec3 color;
	float ambientIntensity;
	float diffuseIntensity;
};

struct DirectionalLight
{
	Light base;
	vec3 direction;
};

struct PointLight
{
	Light base;
	vec3 position;
	float constant;
	float linear;
	float exponent;
};

struct SpotLight
{
	PointLight base;
	vec3 direction;
	float edge;
};

struct Material
{
	float specularIntensity;
	float shininess;
};

uniform int pointLightCount;
uniform int spotLightCount;

uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform sampler2D theTexture;
uniform Material material;

uniform vec3 eyePosition;

//modelo de Phong: ambiental + difusa + especular
vec4 CalcLightByDirection(Light light, vec3 direction)
{
	vec4 ambientcolor = vec4(light.color, 1.0f) * light.ambientIntensity;
	//producto punto entre normal y direccion de la luz = coseno del angulo (por eso se normalizan los dos)
	//direction va de la luz hacia el fragmento, se niega para compararla con la normal, que apunta hacia afuera
	float diffuseFactor = max(dot(normalize(Normal), -normalize(direction)), 0.0f);
	vec4 diffusecolor = vec4(light.color * light.diffuseIntensity * diffuseFactor, 1.0f);

	vec4 specularcolor = vec4(0, 0, 0, 0);

	if(diffuseFactor > 0.0f)
	{
		//solo hay brillo especular si la luz le pega a la cara, y depende de donde esta la camara
		vec3 fragToEye = normalize(eyePosition - FragPos);
		vec3 reflectedVertex = normalize(reflect(direction, normalize(Normal)));

		float specularFactor = dot(fragToEye, reflectedVertex);
		if(specularFactor > 0.0f)
		{
			specularFactor = pow(specularFactor, material.shininess);
			specularcolor = vec4(light.color * material.specularIntensity * specularFactor, 1.0f);
		}
	}

	return (ambientcolor + diffusecolor + specularcolor);
}

vec4 CalcDirectionalLight()
{
	return CalcLightByDirection(directionalLight.base, directionalLight.direction);
}

vec4 CalcPointLight(PointLight pLight)
{
	vec3 direction = FragPos - pLight.position;
	float distance = length(direction);
	direction = normalize(direction);

	vec4 color = CalcLightByDirection(pLight.base, direction);
	//atenuacion: entre mas lejos, mas se divide la luz
	float attenuation = pLight.exponent * distance * distance +
						pLight.linear * distance +
						pLight.constant;

	return (color / attenuation);
}

vec4 CalcSpotLight(SpotLight sLight)
{
	vec3 rayDirection = normalize(FragPos - sLight.base.position);
	float slFactor = dot(rayDirection, sLight.direction);

	//si el fragmento esta dentro del cono se ilumina como luz puntual, y se va difuminando hacia el borde
	if(slFactor > sLight.edge)
	{
		vec4 color = CalcPointLight(sLight.base);

		return color * (1.0f - (1.0f - slFactor)*(1.0f/(1.0f - sLight.edge)));

	} else {
		return vec4(0, 0, 0, 0);
	}
}

vec4 CalcPointLights()
{
	vec4 totalcolor = vec4(0, 0, 0, 0);
	for(int i = 0; i < pointLightCount; i++)
	{
		totalcolor += CalcPointLight(pointLights[i]);
	}

	return totalcolor;
}

vec4 CalcSpotLights()
{
	vec4 totalcolor = vec4(0, 0, 0, 0);
	for(int i = 0; i < spotLightCount; i++)
	{
		totalcolor += CalcSpotLight(spotLights[i]);
	}

	return totalcolor;
}


void main()
{
	vec4 finalcolor = CalcDirectionalLight();
	finalcolor += CalcPointLights();
	finalcolor += CalcSpotLights();
	//color final = textura * color del objeto (uniform color) * toda la luz que le llega
	color = texture(theTexture, TexCoord)*vColor*finalcolor;
}
