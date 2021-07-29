(: Name: CondExpr009 :)
(: Description: or-expression in test expression :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

if  ($input-context/Root/Customers[1]/@CustomerID = 'ALFKI' or $input-context/Root/Customers[1]/FullAddress/City = 'Non-Existent')
then                 
     <PASS/>
else
      <FAIL/>
