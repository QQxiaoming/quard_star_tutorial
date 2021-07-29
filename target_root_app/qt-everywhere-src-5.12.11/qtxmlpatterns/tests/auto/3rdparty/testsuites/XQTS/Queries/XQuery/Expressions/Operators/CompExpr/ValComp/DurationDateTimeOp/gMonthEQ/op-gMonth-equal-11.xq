(:*******************************************************:)
(:Test: op-gMonth-equal-11                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "gMonth-equal" function used    :)
(:together with "fn:true"/or expression (eq operator).   :)
(:*******************************************************:)
 
(xs:gMonth("--03Z") eq xs:gMonth("--01Z")) or (fn:true())