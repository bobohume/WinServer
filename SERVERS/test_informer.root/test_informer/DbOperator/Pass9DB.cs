using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;

namespace DbOperator
{
    public class Pass9DB
    {
        static string ConnStr = "metadata=res://*/Pass9DB.csdl|res://*/Pass9DB.ssdl|res://*/Pass9DB.msl;provider=System.Data.SqlClient;provider connection string=&quot;Data Source=192.168.0.200;Initial Catalog=PASS9;User ID=sa;Password=firerain;MultipleActiveResultSets=True&quot;";

       
        public static int AddAccount( string name, out int sequence )
        {
            sequence = 0;

            using (PASS9 db = new PASS9())
            {
                try
                {
                    var accounts = from a in db.Tbl_Account where a.Name == name select a;
                    if (accounts.Count() > 0)
                        return 0;

                    Tbl_Account account = new Tbl_Account();
                    account.Name = name;
                    account.ActiveCode = "";
                    db.AddToTbl_Account(account);
                    Tbl_Record rec = new Tbl_Record();
                    db.AddToTbl_Record(rec);
                    db.SaveChanges();
                    sequence = rec.Id;
                    return account.Id;
                }
                catch( Exception e )
                {
                    return 0;
                }
            }
        }

        public static int InverseActive( string name, int zoneId, out int sequence )
        {
            sequence = 0;

            using (PASS9 db = new PASS9())
            {
                try
                {
                    var accounts = from a in db.Tbl_Account where a.ZoneId == 0 && a.Name == name select a;
                    if (accounts.Count() == 0)
                        return 0;

                    Tbl_Account account = accounts.First();
                    account.ZoneId = zoneId;
                    Tbl_Record rec = new Tbl_Record();
                    db.AddToTbl_Record(rec);
                    db.SaveChanges();
                    sequence = rec.Id;
                    return account.Id;
                }
                catch
                {
                    return 0;
                }
            }

        }

        public static int ActiveAccount( string name, string activeCode, out int sequence )
        {
            sequence = 0;

            using (PASS9 db = new PASS9())
            {
                try
                {
                    var accounts = from a in db.Tbl_Account where a.Name == name && a.ActiveCode == "" select a;
                    
                    if (accounts.Count() == 0)
                        return 0;
                    
                    Tbl_Account account = accounts.First();
                    account.ActiveCode = activeCode;
                    Tbl_Record rec = new Tbl_Record();
                    db.AddToTbl_Record(rec);
                    db.SaveChanges();
                    sequence = rec.Id;
                    return account.Id;
                }
                catch
                {
                    return 0;
                }
            }
        }
    }
}
