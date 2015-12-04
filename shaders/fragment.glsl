#version 330

// varying variables
in vec4 fColor;
in vec3 fTexCoord;
in vec3 fNormal;
in vec3 fPosition;
noperspective in vec3 dist;

// output color
out vec4 outputF;

uniform int pass;	// 1 -- 1st pass / 2 -- 2nd pass

// uniform variables
uniform int textureEnabled;		// 1 -- texture / 0 -- color only
//uniform int wireframeEnalbed;	// 0 -- no wireframe / 1 -- add wireframe
uniform int depthComputation;  // 1 -- depth computation / 0 - otherwise

uniform int useShadow;		// 1 -- use shadow / 0 - no shadow 
uniform mat4 light_mvpMatrix;
uniform vec3 lightDir;
uniform sampler2D tex0;
uniform sampler2D shadowMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;
uniform int screenWidth;
uniform int screenHeight;
uniform int renderingMode;		// 1 -- regular / 2 -- wireframe / 3 -- line / 4 -- sketchy
//uniform int lineRendering;     // 1 -- line rendering / 0 - otherwise
uniform float depthSensitivity;
uniform float normalSensitivity;
uniform int useThreshold;	// 1 -- use threshold / 0 -- otherwise
uniform float threshold;
uniform int seed;

float shadowCoef(){
	vec4 shadow_coord2 = light_mvpMatrix * vec4(fPosition, 1.0);
	vec3 ProjCoords = shadow_coord2.xyz / shadow_coord2.w;
	vec2 UVCoords;
	UVCoords.x = 0.5 * ProjCoords.x + 0.5;
	UVCoords.y = 0.5 * ProjCoords.y + 0.5;
	float z = 0.5 * ProjCoords.z + 0.5;

	float visibility = 1.0f;
	if (texture2D(shadowMap, UVCoords).z  <  z) {
		visibility = 0;
	}
	return visibility;
}

void regularRendering() {
	// for color mode
	float opacity = fColor.w;
	outputF = vec4(fColor.xyz, 1);

	// depth computation
	if (depthComputation == 1) return;

	if (textureEnabled == 1) { // for texture mode
		outputF = outputF * texture(tex0, fTexCoord.rg);
	}

	// lighting
	vec4 ambient = vec4(0.6, 0.6, 0.6, 1.0);
	//vec4 diffuse = vec4(0.8, 0.8, 0.8, 1.0) * max(0.0, dot(-lightDir, fNormal));
	vec4 diffuse = vec4(0.4, 0.4, 0.4, 1.0) * max(0.0, max(dot(vec3(0.6, 0.1, 0.95), fNormal), dot(vec3(-0.2, 0.88, 0.1), fNormal)));

	float shadow_coef = 1.0;
	if (useShadow == 1) {
		shadow_coef = shadowCoef();
	}
	outputF = (ambient + (shadow_coef * 0.95 + 0.05) * diffuse) * outputF;

	outputF.w = opacity;
}

void wireframeRendering() {
	// for color mode
	float opacity = fColor.w;
	outputF = vec4(fColor.xyz, 1);

	// determine frag distance to closest edge
	float nearD = min(min(dist[0], dist[1]), dist[2]);
	float edgeIntensity;
	if (nearD < 1.0) {
		edgeIntensity = 1.0;
	}
	else {
		edgeIntensity = exp2(-1.0*(nearD - 1)*(nearD - 1));
	}

	if (textureEnabled == 1) { // for texture mode
		outputF = outputF * texture(tex0, fTexCoord.rg);
	}

	// lighting
	vec4 ambient = vec4(0.6, 0.6, 0.6, 1.0);
	vec4 diffuse = vec4(0.8, 0.8, 0.8, 1.0) * max(0.0, dot(-lightDir, fNormal));

	float shadow_coef = 1.0;
	if (useShadow == 1) {
		shadow_coef = shadowCoef();
	}
	outputF = (ambient + (shadow_coef * 0.95 + 0.05) * diffuse) * outputF;

	outputF = edgeIntensity * vec4(0.05, 0.05, 0.05, 1.0) + (1.0 - edgeIntensity) * outputF;

	outputF.w = opacity;
}

void lineRendering() {
	float normal_diff = 0;
	float depth_diff = 0;
	int range = 1;

	// difference in normal between this pixel and the neighbor pixels
	vec3 n = texture(normalMap, vec2(gl_FragCoord.x / screenWidth, gl_FragCoord.y / screenHeight)).xyz;
	float n2 = texture(normalMap, vec2(gl_FragCoord.x / screenWidth, gl_FragCoord.y / screenHeight)).w;
	float d = texture(depthMap, vec2(gl_FragCoord.x / screenWidth, gl_FragCoord.y / screenHeight)).x;

	for (int xx = -range; xx <= range; ++xx) {
		for (int yy = -range; yy <= range; ++yy) {
			if (xx == 0 && yy == 0) continue;

			vec3 nn = texture(normalMap, vec2((gl_FragCoord.x + xx) / screenWidth, (gl_FragCoord.y + yy) / screenHeight)).xyz;
			float nn2 = texture(normalMap, vec2((gl_FragCoord.x + xx) / screenWidth, (gl_FragCoord.y + yy) / screenHeight)).w;
			if (nn.x == 0 && nn.y == 0 && nn.z == 0) {
				normal_diff = normalSensitivity;
			}
			else {
				normal_diff = max(normal_diff, length(nn - n) * normalSensitivity);
				depth_diff = max(depth_diff, abs(nn2 - n2) * depthSensitivity);
			}

			/*float dd = texture(depthMap, vec2((gl_FragCoord.x+xx) / screenWidth, (gl_FragCoord.y+yy) / screenHeight)).x;
			depth_diff = max(depth_diff, abs(dd - d) * depthSensitivity);*/
		}
	}

	float diff = max(normal_diff, depth_diff);
	diff = min(1.0, diff);
	if (useThreshold == 1) {
		if (diff < threshold) {
			diff = 0.0;
		}
		else {
			diff = 1.0;
		}
	}
	outputF = vec4(1 - diff, 1 - diff, 1 - diff, 1);
}

// return random value in [-1, 1]
float random(vec3 position, float scale, int cycle_size) {
	return float(int((position.x + position.y + position.z + seed) * scale) % (cycle_size * 2 + 1) - cycle_size) / float(cycle_size);
}

void sketchyRendering() {
	float diff = 0;
	int range = 1;

	int num_iterations = 2;
	float scale_x[3] = float[](1.53, 3.17, 2.37);
	float scale_y[3] = float[](1.71, 2.57, 2.97);
	float scale_z[3] = float[](1.43, 2.17, 2.77);
	int cycle_size[3] = int[](273, 193, 311);

	float jitter_size = 2.3;

	for (int iter = 0; iter < num_iterations; ++iter) {
		float sx = gl_FragCoord.x + random(gl_FragCoord.xyz + fPosition * 30, scale_x[iter], cycle_size[iter]) * jitter_size;
		float sy = gl_FragCoord.y + random(gl_FragCoord.xyz + fPosition * 30, scale_y[iter], cycle_size[iter]) * jitter_size;
		float sz = gl_FragCoord.z + random(gl_FragCoord.xyz + fPosition * 30, scale_z[iter], cycle_size[iter]) * jitter_size;

		// difference in normal between this pixel and the neighbor pixels
		vec3 n = texture(normalMap, vec2(sx / screenWidth, sy / screenHeight)).xyz;
		float n2 = texture(normalMap, vec2(gl_FragCoord.x / screenWidth, gl_FragCoord.y / screenHeight)).w;
		float d = texture(depthMap, vec2(sx / screenWidth, sy / screenHeight)).x;

		for (int xx = -range; xx <= range; ++xx) {
			for (int yy = -range; yy <= range; ++yy) {
				if (xx == 0 && yy == 0) continue;

				vec3 nn = texture(normalMap, vec2((sx + xx) / screenWidth, (sy + yy) / screenHeight)).xyz;
				float nn2 = texture(normalMap, vec2((gl_FragCoord.x + xx) / screenWidth, (gl_FragCoord.y + yy) / screenHeight)).w;
				if (nn.x == 0 && nn.y == 0 && nn.z == 0) {
					diff = normalSensitivity;
				}
				else {
					diff = max(diff, length(nn - n) * normalSensitivity);
					diff = max(diff, abs(nn2 - n2) * depthSensitivity);
				}

				/*
				float dd = texture(depthMap, vec2((sx+xx) / screenWidth, (sy+yy) / screenHeight)).x;
				diff = max(diff, abs(dd - d) * depthSensitivity);
				*/
			}
		}
	}

	diff = min(1, diff);
	if (diff < threshold) {
		diff = 0;
	}
	else {
		diff = 1;
	}
	outputF = vec4(1 - diff, 1 - diff, 1 - diff, 1);
}

void main() {
	if (pass == 1) {
		outputF = vec4((fNormal + 1) * 0.5, abs(dot(fPosition, fNormal)));
	}
	else {
		if (renderingMode == 1) {
			regularRendering();
		}
		else if (renderingMode == 2) {
			wireframeRendering();
		}
		else if (renderingMode == 3) {
			lineRendering();
		}
		else if (renderingMode == 4) {
			sketchyRendering();
		}
	}
}

