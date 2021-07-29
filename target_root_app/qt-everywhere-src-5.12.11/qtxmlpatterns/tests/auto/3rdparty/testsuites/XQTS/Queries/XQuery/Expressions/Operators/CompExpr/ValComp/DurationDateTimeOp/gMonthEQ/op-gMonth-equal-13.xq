(:*******************************************************:)
(:Test: op-gMonth-equal-13                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "gMonth-equal" function used    :)
(:together with "fn:false"/or expression (eq operator).  :)
(:*******************************************************:)
 
(xs:gMonth("--05Z") eq xs:gMonth("--05Z")) or (fn:false())