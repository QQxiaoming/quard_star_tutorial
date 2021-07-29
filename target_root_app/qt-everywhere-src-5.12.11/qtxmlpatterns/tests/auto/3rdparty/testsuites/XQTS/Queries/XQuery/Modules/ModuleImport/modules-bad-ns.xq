(:*******************************************************:)
(: Test: modules-bad-ns.xq                               :)
(: Written By: Mary Holstege                             :)
(: Date: 2005/12/05 14:46:04                             :)
(: Purpose: Negative test: wrong namespace               :)
(:*******************************************************:)

(: insert-start :)
import module namespace test2="http://www.w3.org/TestModules/test2";
declare variable $input-context external;
(: insert-end :)

<result>{test2:ok()}</result>