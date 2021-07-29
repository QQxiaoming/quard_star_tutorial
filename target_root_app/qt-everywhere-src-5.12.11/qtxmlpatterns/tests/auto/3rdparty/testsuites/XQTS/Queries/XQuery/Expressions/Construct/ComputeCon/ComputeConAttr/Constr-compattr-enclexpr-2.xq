(: Name: Constr-compattr-enclexpr-2 :)
(: Written by: Andreas Behm :)
(: Description: enclosed expression in attribute content - nodes :)

element elem {attribute attr {<elem>123</elem>, (<elem attr='456'/>)/@attr, (<elem>789</elem>)/text()}}
