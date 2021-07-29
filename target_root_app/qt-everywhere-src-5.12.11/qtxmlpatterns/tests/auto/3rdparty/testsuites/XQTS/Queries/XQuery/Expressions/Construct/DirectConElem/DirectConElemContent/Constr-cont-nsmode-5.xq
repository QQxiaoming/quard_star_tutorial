(: Name: Constr-cont-nsmode-5 :)
(: Written by: Andreas Behm :)
(: Description: Copy namespace sensitive content :)

declare copy-namespaces no-preserve, inherit;
declare construction preserve;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<new>{ $input-context//*:QName }</new>
