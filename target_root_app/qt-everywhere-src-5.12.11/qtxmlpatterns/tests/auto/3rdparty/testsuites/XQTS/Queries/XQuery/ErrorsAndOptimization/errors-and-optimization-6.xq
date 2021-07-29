(: Name: errors-and-optimization-6.xq :)
(: Description: 

   Expressions must not be rewritten in such a way as to create or
   remove static errors. 
   
:)

(: insert-start :)
(: insert-end :)

for $s in "var:QName"
return QName($s)
