(: Name: CondExpr007 :)
(: Description: else-expr has another if expression :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

if ($input-context//MissingNode) then                 
     <elem3/>
else
         if ($input-context/Root/Customers[@CustomerID='ALFKI']//Country = "Germany") then
                 <elem1/>
         else
                 <elem2/>
