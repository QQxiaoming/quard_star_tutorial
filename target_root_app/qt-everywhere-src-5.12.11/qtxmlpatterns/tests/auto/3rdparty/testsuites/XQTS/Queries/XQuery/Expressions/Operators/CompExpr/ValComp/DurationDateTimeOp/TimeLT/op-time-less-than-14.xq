(:*******************************************************:)
(:Test: op-time-less-than-14                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "time-less-than" function used  :)
(:together with "fn:false"/or expression (le operator).  :)
(:*******************************************************:)
 
(xs:time("13:00:00Z") le xs:time("17:00:00Z")) or (fn:false())