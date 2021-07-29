(: Name: NodeTesthc-7 :)
(: Description: Simple test for Element node types involving a boolean expression (and fn:false) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<result>
  {$input-context1//overtime/node() and fn:false()}
</result>
