(: Name: Parenexpr-15 :)
(: Written by: Andreas Behm :)
(: Description: union and except :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<elem>{$input-context//node() | ($input-context//node() except $input-context//comment())}</elem>
