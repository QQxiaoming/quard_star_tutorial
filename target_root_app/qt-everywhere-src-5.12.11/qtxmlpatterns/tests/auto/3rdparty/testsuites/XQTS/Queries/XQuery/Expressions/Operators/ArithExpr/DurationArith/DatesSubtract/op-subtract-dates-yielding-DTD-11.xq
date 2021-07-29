(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-11                :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator used  :)
(:as part of a "div" expression.                         :)
(:*******************************************************:)
 
(xs:date("1978-12-12Z") - xs:date("1978-12-12Z")) div xs:dayTimeDuration("P17DT10H02M")