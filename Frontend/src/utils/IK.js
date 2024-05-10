// Definition of geometric constants of the robot
const e = 173.21;     // Length of the end effector
const f = 346.41;    // Length of the base
const re = 400.0; // Length of the end effector linkages
const rf = 150.0; // Length of the base linkages

// Definition of trigonometric constants
const sqrt3 = Math.sqrt(3.0);
const pi = Math.PI;
const sin120 = sqrt3 / 2.0;
const cos120 = -0.5;
const tan60 = sqrt3;
const sin30 = 0.5;
const tan30 = 1 / sqrt3;

// Function to calculate the angle theta for the YZ plane
function delta_calcAngleYZ(x0, y0, z0) {
  const y1 = -0.5 * 0.57735 * f; // Calculation based on f/2 * tan(30)
  y0 -= 0.5 * 0.57735 * e; // Shift from center to edge of end effector

  const a = (x0 * x0 + y0 * y0 + z0 * z0 + rf * rf - re * re - y1 * y1) / (2 * z0);
  const b = (y1 - y0) / z0;
  const d = -(a + b * y1) * (a + b * y1) + rf * (b * b * rf + rf);

  if (d < 0) return { status: -1, theta: null }; // Non-existent point if d < 0

  const yj = (y1 - a * b - Math.sqrt(d)) / (b * b + 1);
  const zj = a + b * yj;
  const theta = 180.0 * Math.atan(-zj / (y1 - yj)) / pi + (yj > y1 ? 180.0 : 0.0);

  return { status: 0, theta };
}
// Inverse Kinematics
function delta_calcInverse(x0, y0, z0) {
  let theta1, theta2, theta3;

  let result = delta_calcAngleYZ(x0, y0, z0);
  if (result.status !== 0) return { status: result.status };

  theta1 = result.theta;
  result = delta_calcAngleYZ(x0 * cos120 + y0 * sin120, y0 * cos120 - x0 * sin120, z0);
  if (result.status !== 0) return { status: result.status };

  theta2 = result.theta;
  result = delta_calcAngleYZ(x0 * cos120 - y0 * sin120, y0 * cos120 + x0 * sin120, z0);
  if (result.status !== 0) return { status: result.status };

  theta3 = result.theta;
  return { status: 0, theta1, theta2, theta3 };
}

export default delta_calcInverse ;
