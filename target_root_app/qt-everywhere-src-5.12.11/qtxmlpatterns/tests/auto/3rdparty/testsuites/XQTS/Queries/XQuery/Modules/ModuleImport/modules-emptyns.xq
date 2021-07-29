(:*******************************************************:)
(: Test: modules-emptyns.xqy                             :)
(: Written By: Mary Holstege                             :)
(: Date: 2005/12/05 14:46:04                             :)
(: Purpose: Module with an empty namespace               :)
(:*******************************************************:)

(: insert-start :)
import module namespace test1= "";
declare variable $input-context external;
(: insert-end :)

<result>ok</result>
