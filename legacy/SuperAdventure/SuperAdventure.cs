using System.Windows.Forms;
using Engine;

namespace SuperAdventure
{
    public partial class SuperAdventure : Form
    {
        private Player _player;

        public SuperAdventure()
        {
            InitializeComponent();

            _player = new Player(10, 10, 20, 0, 1);

            lblHitPoints.Text = _player.CurrentHitPoints.ToString();
            lblGold.Text = _player.Gold.ToString();
            lblExperience.Text = _player.ExperiencePoints.ToString();
            lblLevel.Text = _player.Level.ToString();

            Location location = new Location(1, "Home", "This is your home.");
        }

    }
}
