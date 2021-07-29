(:*******************************************************:)
(: Test: modules-two-import-ok.xq                        :)
(: Written By: Mary Holstege                             :)
(: Date: 2005/12/05 14:46:04                             :)
(: Purpose: Importing two modules, the approved way      :)
(:*******************************************************:)

(: insert-start :)
import module namespace test1="http://www.w3.org/TestModules/test1";
declare variable $input-context external;
(: insert-end :)

<result>{test1:ok()}</result>
