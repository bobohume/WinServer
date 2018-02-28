using DbOperator;
using Microsoft.VisualStudio.TestTools.UnitTesting;
namespace DbOperator.Test
{
    
    
    /// <summary>
    ///这是 Pass9DBTest 的测试类，旨在
    ///包含所有 Pass9DBTest 单元测试
    ///</summary>
    [TestClass()]
    public class Pass9DBTest
    {


        private TestContext testContextInstance;

        /// <summary>
        ///获取或设置测试上下文，上下文提供
        ///有关当前测试运行及其功能的信息。
        ///</summary>
        public TestContext TestContext
        {
            get
            {
                return testContextInstance;
            }
            set
            {
                testContextInstance = value;
            }
        }

        #region 附加测试属性
        // 
        //编写测试时，还可使用以下属性:
        //
        //使用 ClassInitialize 在运行类中的第一个测试前先运行代码
        //[ClassInitialize()]
        //public static void MyClassInitialize(TestContext testContext)
        //{
        //}
        //
        //使用 ClassCleanup 在运行完类中的所有测试后再运行代码
        //[ClassCleanup()]
        //public static void MyClassCleanup()
        //{
        //}
        //
        //使用 TestInitialize 在运行每个测试前先运行代码
        //[TestInitialize()]
        //public void MyTestInitialize()
        //{
        //}
        //
        //使用 TestCleanup 在运行完每个测试后运行代码
        //[TestCleanup()]
        //public void MyTestCleanup()
        //{
        //}
        //
        #endregion


        /// <summary>
        ///InverseActive 的测试
        ///</summary>
        [TestMethod()]
        public void InverseActiveTest()
        {
            string name = string.Empty; // TODO: 初始化为适当的值
            int zoneId = 0; // TODO: 初始化为适当的值
            int sequence = 0; // TODO: 初始化为适当的值
            int sequenceExpected = 0; // TODO: 初始化为适当的值
            int expected = 0; // TODO: 初始化为适当的值
            int actual;
            actual = Pass9DB.InverseActive(name, zoneId, out sequence);
            Assert.AreEqual(sequenceExpected, sequence);
            Assert.AreEqual(expected, actual);
            Assert.Inconclusive("验证此测试方法的正确性。");
        }

        /// <summary>
        ///AddAccount 的测试
        ///</summary>
        [TestMethod()]
        public void AddAccountTest()
        {
            string name = string.Empty; // TODO: 初始化为适当的值
            int sequence = 0; // TODO: 初始化为适当的值
            int sequenceExpected = 0; // TODO: 初始化为适当的值
            int expected = 0; // TODO: 初始化为适当的值
            int actual;
            actual = Pass9DB.AddAccount(name, out sequence);
            Assert.AreEqual(sequenceExpected, sequence);
            Assert.AreEqual(expected, actual);
            Assert.Inconclusive("验证此测试方法的正确性。");
        }

        /// <summary>
        ///ActiveAccount 的测试
        ///</summary>
        [TestMethod()]
        public void ActiveAccountTest()
        {
            string name = string.Empty; // TODO: 初始化为适当的值
            string activeCode = string.Empty; // TODO: 初始化为适当的值
            int sequence = 0; // TODO: 初始化为适当的值
            int sequenceExpected = 0; // TODO: 初始化为适当的值
            int expected = 0; // TODO: 初始化为适当的值
            int actual;
            actual = Pass9DB.ActiveAccount(name, activeCode, out sequence);
            Assert.AreEqual(sequenceExpected, sequence);
            Assert.AreEqual(expected, actual);
            Assert.Inconclusive("验证此测试方法的正确性。");
        }

        /// <summary>
        ///Pass9DB 构造函数 的测试
        ///</summary>
        [TestMethod()]
        public void Pass9DBConstructorTest()
        {
            Pass9DB target = new Pass9DB();
            Assert.Inconclusive("TODO: 实现用来验证目标的代码");
        }
    }
}
