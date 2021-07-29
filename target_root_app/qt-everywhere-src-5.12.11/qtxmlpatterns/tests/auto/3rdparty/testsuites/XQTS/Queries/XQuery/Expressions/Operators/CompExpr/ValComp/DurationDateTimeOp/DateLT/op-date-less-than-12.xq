(:*******************************************************:)
(:Test: op-date-less-than-12                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-less-than" function used  :)
(:together with "fn:true"/or expression (le operator).   :)
(:*******************************************************:)
 
(xs:date("2000-10-25Z") le xs:date("2000-10-26Z")) or (fn:true())