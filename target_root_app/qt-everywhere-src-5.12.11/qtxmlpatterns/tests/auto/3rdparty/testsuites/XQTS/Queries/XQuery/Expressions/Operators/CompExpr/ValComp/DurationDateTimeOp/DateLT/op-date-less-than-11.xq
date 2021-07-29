(:*******************************************************:)
(:Test: op-date-less-than-11                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-less-than" function used  :)
(:together with "fn:true"/or expression (lt operator).   :)
(:*******************************************************:)
 
(xs:date("1980-05-18Z") lt xs:date("1980-05-17Z")) or (fn:true())