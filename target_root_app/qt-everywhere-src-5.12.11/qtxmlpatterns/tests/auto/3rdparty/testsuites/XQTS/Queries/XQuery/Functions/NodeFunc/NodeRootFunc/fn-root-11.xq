(: Name: fn-root-11:)
(: Description: Evaluation of the fn:root function with argument set to an computed element with attribute node by setting directly on argument.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:root(element anElement {attribute anAttribute {"Attribute Value"}})