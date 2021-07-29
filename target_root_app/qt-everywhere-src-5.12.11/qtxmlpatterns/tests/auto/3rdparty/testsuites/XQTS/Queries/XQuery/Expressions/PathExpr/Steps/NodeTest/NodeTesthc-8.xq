(: Name: NodeTesthc-8 :)
(: Description: Simple test for Element node types involving a boolean expression (and fn:true) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<result>
  {$input-context1//overtime/node() and fn:true()}
</result>
