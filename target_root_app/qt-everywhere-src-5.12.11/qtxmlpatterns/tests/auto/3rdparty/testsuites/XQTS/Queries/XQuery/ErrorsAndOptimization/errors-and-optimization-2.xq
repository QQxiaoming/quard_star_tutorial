(: Name: errors-and-optimization-2.xq :)
(: Description: 

   Conditional expressions must not return the value delivered by a
   branch unless that branch is selected.
:)

(: insert-start :)
(: insert-end :)

if (true()) 
   then 1 div 0
   else 1
