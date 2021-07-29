(:*******************************************************:)
(:Test: op-date-less-than-14                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-less-than" function used  :)
(:together with "fn:false"/or expression (le operator).  :)
(:*******************************************************:)
 
(xs:date("1980-10-25Z") le xs:date("1980-10-26Z")) or (fn:false())