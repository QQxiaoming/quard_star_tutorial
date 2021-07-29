(:*******************************************************:)
(:Test: op-gMonth-equal-14                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "gMonth-equal" function used    :)
(:together with "fn:false"/or expression (ne operator).  :)
(:*******************************************************:)
 
(xs:gMonth("--09Z") ne xs:gMonth("--09Z")) or (fn:false())