(:*******************************************************:)
(:Test: op-date-greater-than-12                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-greater-than" function used:)
(:together with "fn:true"/or expression (ge operator).   :)
(:*******************************************************:)
 
(xs:date("2000-10-25Z") ge xs:date("2000-10-26Z")) or (fn:true())