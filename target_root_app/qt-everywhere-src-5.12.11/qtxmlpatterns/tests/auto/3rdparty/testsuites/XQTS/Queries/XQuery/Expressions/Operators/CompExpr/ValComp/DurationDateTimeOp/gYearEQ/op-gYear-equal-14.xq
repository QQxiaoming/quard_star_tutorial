(:*******************************************************:)
(:Test: op-gYear-equal-14                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYear-equal" function used     :)
(:together with "fn:false"/or expression (ne operator).  :)
(:*******************************************************:)
 
(xs:gYear("1980Z") ne xs:gYear("1980Z")) or (fn:false())