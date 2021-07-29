(: Name: sequenceexpressionhc9 :)
(: Description: Constructing Sequences. Sequence expression resulting by quering xml file string data (multiple xml sources):)

(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

($input-context1//empnum,$input-context2//empname)