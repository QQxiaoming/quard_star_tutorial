(:*******************************************************:)
(:Test: op-date-greater-than-11                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-greater-than" function used:)
(:together with "fn:true"/or expression (gt operator).   :)
(:*******************************************************:)
 
(xs:date("1980-05-18Z") gt xs:date("1980-05-17Z")) or (fn:true())