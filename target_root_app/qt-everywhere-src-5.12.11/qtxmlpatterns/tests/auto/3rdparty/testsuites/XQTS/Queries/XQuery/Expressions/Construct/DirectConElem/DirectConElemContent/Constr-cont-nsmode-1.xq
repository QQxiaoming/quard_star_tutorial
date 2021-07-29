(: Name: Constr-cont-nsmode-1 :)
(: Written by: Andreas Behm :)
(: Description: Copy-namespace mode preserve, inherit :)

declare copy-namespaces preserve, inherit;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<y xmlns:inherit="http://www.example.com/inherit">{$input-context}</y>/x/z
