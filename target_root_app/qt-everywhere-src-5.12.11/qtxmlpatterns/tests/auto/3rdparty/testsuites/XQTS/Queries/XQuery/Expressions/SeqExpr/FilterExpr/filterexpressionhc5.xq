(: Name: filterexpressionhc5 :)
(: Description: Simple filter expression as a stept in a path expression involving the "last" function:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1//employee[fn:last()])