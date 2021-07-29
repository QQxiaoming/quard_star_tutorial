(: Name: NodeTesthc-5 :)
(: Description: Simple test for Element node types involving a boolean expression (or fn:false) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<result>
  {$input-context1//overtime/node() or fn:false()}
</result>
