(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-time-11                :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-time" function used  :)
(:together with a "div" expression.                      :)
(:*******************************************************:)

(xs:time("01:01:01Z") + xs:dayTimeDuration("P10DT10H10M")) div (xs:time("01:01:01Z") + xs:dayTimeDuration("P10DT10H10M"))