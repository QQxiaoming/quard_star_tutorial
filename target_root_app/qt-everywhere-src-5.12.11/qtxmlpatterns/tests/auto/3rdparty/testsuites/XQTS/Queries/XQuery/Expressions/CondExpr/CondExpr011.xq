(: FileName: CondExpr011 :)
(: Purpose: If expression as argument to a function :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<out>{fn:string-length(if (2 != 3) then 'foo' else 'expanded-foo')}</out>