(:*******************************************************:)
(: Test: modules-schema-contex.xq                        :)
(: Written By: Mary Holstege                             :)
(: Date: 2005/12/05 14:46:04                             :)
(: Purpose: Test a module with a schema import.          :)
(:*******************************************************:)

(: insert-start :)
import module namespace ctx="http://www.w3.org/TestModules/context";
declare variable $input-context external;
(: insert-end :)

<result>{ctx:use-schema() instance of xs:int}</result>
