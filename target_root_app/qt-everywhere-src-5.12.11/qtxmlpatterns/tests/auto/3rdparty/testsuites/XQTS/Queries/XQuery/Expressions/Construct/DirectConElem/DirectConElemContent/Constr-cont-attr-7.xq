(: Name: Constr-cont-attr-7 :)
(: Written by: Andreas Behm :)
(: Description: attribute in content with same name :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<elem>{$input-context//west/@mark, $input-context//center/@mark}</elem>
