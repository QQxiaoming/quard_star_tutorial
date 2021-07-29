(:*******************************************************:)
(: Test: test1a-lib.xq                                   :)
(: Written By: Mary Holstege                             :)
(: Date: 2005/12/05 14:47:00                             :)
(: Purpose: Simple library module                        :)
(:*******************************************************:)

module namespace test1="http://www.w3.org/TestModules/test1";

declare function test1:hello-world ()
{
   "hello world"
};
