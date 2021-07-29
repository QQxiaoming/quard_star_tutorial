(:*******************************************************:)
(:Test: op-date-less-than-13                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-less-than" function used  :)
(:together with "fn:false"/or expression (lt operator).  :)
(:*******************************************************:)
 
(xs:date("1980-01-01Z") lt xs:date("1980-10-01Z")) or (fn:false())