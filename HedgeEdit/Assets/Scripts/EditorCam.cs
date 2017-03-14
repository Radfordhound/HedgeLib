using UnityEngine;

public class EditorCam : MonoBehaviour
{
    //Variables/Constants
    public const float StartSpeed = 0.01f, SpeedCap = 1f,
        FastSpeedCap = 4f, Accel = 0.05f;

    private float pitch = 0, yaw = 0, spd = StartSpeed;
    private bool hasMoved = false;

    //Unity Events
    private void Update()
    {
        bool isRBDown = Input.GetMouseButton(1);
        bool fastMove = Input.GetKey(KeyCode.LeftShift);
        hasMoved = false;

        Cursor.visible = !isRBDown;
        Cursor.lockState = (isRBDown) ?
            CursorLockMode.Locked : CursorLockMode.None;

        if (isRBDown)
        {
            //Rotation
            yaw += 2 * Input.GetAxis("Mouse X");
            pitch -= 2 * Input.GetAxis("Mouse Y");

            transform.eulerAngles = new Vector3(pitch, yaw, 0);

            //Forward/Backwards
            if (Input.GetKey(KeyCode.W))
            {
                Move(transform.forward, false, fastMove);
            }
            else if (Input.GetKey(KeyCode.S))
            {
                Move(transform.forward, true, fastMove);
            }

            //Left/Right
            if (Input.GetKey(KeyCode.D))
            {
                Move(transform.right, false, fastMove);
            }
            else if (Input.GetKey(KeyCode.A))
            {
                Move(transform.right, true, fastMove);
            }
        }

        //Lerp movement speed to zero.
        if (!hasMoved)
        {
            Mathf.Lerp(spd, 0, Accel);
        }
	}

    //Methods
    private void Move(Vector3 direction, bool reverse, bool fastMove)
    {
        float sign = (reverse) ? -1 : 1;
        float maxSpd = (fastMove) ? FastSpeedCap : SpeedCap;

        transform.position += (direction * spd) * sign;
        spd = Mathf.Min(spd + Accel, maxSpd);
        hasMoved = true;
    }
}