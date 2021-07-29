(: Name: errors-and-optimization-1.xq :)
(: Description: 

   Conditional and typeswitch expressions must not raise a dynamic
   error in respect of subexpressions occurring in a branch that is
   not selected
:)
(: insert-start :)
(: insert-end :)


if (true()) 
   then 1 
   else error(QName('http://www.example.com/errors', 'err:oops'), 
              "Oops, this error should not be raised!")
