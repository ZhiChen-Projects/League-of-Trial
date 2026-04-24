#version 330

uniform sampler2D texture0;
uniform vec2 playerPos;    // Normalized (0.0 to 1.0)
uniform int isBlinded;     

in vec2 fragTexCoord;      // Normalized (0.0 to 1.0)
out vec4 finalColor;

void main() {
    vec4 color = texture(texture0, fragTexCoord);

    if (isBlinded == 1) {
        // REQUIREMENT 5: Logic-based distance check
        // Adjust for 16:9 aspect ratio so the hole is a circle, not an oval
        vec2 diff = (fragTexCoord - playerPos) * vec2(1.77, 1.0); 
        float dist = length(diff);
        float radius = 0.2; // Sight radius (20% of screen height)

        if (dist > radius) {
            finalColor = vec4(0.0, 0.0, 0.0, 1.0); // Pitch Black
        } else {
            float fade = 1.0 - (dist / radius);
            finalColor = vec4(color.rgb * fade, color.a);
        }
    } else {
        finalColor = color;
    }
}