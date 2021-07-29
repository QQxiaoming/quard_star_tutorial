(:*******************************************************:)
(: Test: modules-collide2-fn-002.xq                       :)
(: Written By: Mary Holstege                             :)
(: Date: 2005/12/05 14:46:04                             :)
(: Purpose: Colliding function declarations              :)
(:*******************************************************:)

(: insert-start :)
import module namespace test1="http://www.w3.org/TestModules/test1";
declare variable $input-context external;
(: insert-end :)

declare function test1:ok ()
{
   "ok"
};

<result>{test1:ok()}</result>
