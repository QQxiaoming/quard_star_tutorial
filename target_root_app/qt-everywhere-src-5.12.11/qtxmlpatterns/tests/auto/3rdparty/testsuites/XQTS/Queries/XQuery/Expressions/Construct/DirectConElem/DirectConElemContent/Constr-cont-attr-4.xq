(: Name: Constr-cont-attr-4 :)
(: Written by: Andreas Behm :)
(: Description: attribute node following direct content :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<elem>{$input-context//west/@mark}x{$input-context//west/@west-attr-1}</elem>
