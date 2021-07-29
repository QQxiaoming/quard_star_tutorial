(:*************************************************************:)
(:Test: fn-distinct-values-1                                   :)
(:Purpose: Test Bugzilla #5183,                                :)
(:         [FO] Effect of type promotion in fn:distinct-values :)   
(:*************************************************************:)

let $input := (xs:float('1.0'),
               xs:decimal('1.0000000000100000000001'),
               xs:double( '1.00000000001'),
               xs:float('2.0'),
               xs:decimal('2.0000000000100000000001'),
               xs:double( '2.00000000001'))
let $distinct :=  distinct-values($input)
return (
         (every $n in $input satisfies $n = $distinct)
         and (every $bool in (for $d1 at $p in $distinct
                              for $d2 in $distinct [position() > $p]
                              return $d1 eq $d2)
              satisfies not($bool))
       )
