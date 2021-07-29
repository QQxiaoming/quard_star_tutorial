(: Name: filterexpressionhc4 :)
(: Description: Simple filter expression involving union operator:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1//empnum | $input-context1)//employee[xs:integer(hours[1]) gt 20]