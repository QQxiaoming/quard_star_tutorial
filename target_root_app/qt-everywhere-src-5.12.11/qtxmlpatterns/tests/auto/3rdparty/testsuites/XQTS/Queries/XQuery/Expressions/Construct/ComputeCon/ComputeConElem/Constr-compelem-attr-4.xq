(: Name: Constr-compelem-attr-4 :)
(: Written by: Andreas Behm :)
(: Description: attribute in content with same name :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element elem {$input-context//west/@mark, $input-context//center/@mark}
