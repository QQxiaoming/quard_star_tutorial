(:*******************************************************:)
(:Test: op-gYear-equal-13                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYear-equal" function used     :)
(:together with "fn:false"/or expression (eq operator).  :)
(:*******************************************************:)
 
(xs:gYear("1980Z") eq xs:gYear("1980Z")) or (fn:false())