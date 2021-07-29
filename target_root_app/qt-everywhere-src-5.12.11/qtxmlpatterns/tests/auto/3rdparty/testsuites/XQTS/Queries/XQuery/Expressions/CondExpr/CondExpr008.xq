(: Name: CondExpr008 :)
(: Description: and-expression in test expression :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

if ( $input-context/Root/Customers[1]/@CustomerID = 'ALFKI' and $input-context/Root/Customers[1]/FullAddress/City = 'Berlin')
then                 
     <PASS/>
else
      <FAIL/>
