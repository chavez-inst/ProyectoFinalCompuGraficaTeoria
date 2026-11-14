#version 330

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 vColor;

out vec4 color;

//iguales a src/CommonValues.h
const int MAX_POINT_LIGHTS = 16;
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
//en 1 el objeto se ve con su textura tal cual, sin luz; lo usan las cartas
uniform int sinLuz;

//modelo de Phong, suma ambiental, difusa y especular
vec4 CalcLightByDirection(Light light, vec3 direction)
{
	vec4 ambientcolor = vec4(light.color, 1.0f) * light.ambientIntensity;
	//el producto punto da el coseno, por eso se normalizan las dos
	//direction va de la luz al fragmento, por eso se niega
	float diffuseFactor = max(dot(normalize(Normal), -normalize(direction)), 0.0f);
	vec4 diffusecolor = vec4(light.color * light.diffuseIntensity * diffuseFactor, 1.0f);

	vec4 specularcolor = vec4(0, 0, 0, 0);

	if(diffuseFactor > 0.0f)
	{
		//el brillo depende de donde esta la camara
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
	//atenuacion, mas lejos se divide mas
	float attenuation = pLight.exponent * distance * distance +
						pLight.linear * distance +
						pLight.constant;

	return (color / attenuation);
}

vec4 CalcSpotLight(SpotLight sLight)
{
	vec3 rayDirection = normalize(FragPos - sLight.base.position);
	float slFactor = dot(rayDirection, sLight.direction);

	//dentro del cono como luz puntual, difuminando hacia el borde
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
	//los pixeles casi transparentes no se dibujan, para no tapar lo que esta atras
	vec4 colorTextura = texture(theTexture, TexCoord);
	if (colorTextura.a < 0.1)
		discard;
	//textura por color del objeto, por la luz que le llega
	if (sinLuz == 1)
		color = colorTextura*vColor;
	else
		color = colorTextura*vColor*finalcolor;
}
