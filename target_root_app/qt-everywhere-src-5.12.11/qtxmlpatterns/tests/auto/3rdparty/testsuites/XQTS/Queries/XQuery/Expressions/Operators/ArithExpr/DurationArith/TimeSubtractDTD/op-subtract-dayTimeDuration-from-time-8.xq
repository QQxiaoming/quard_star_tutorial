(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-time-8          :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "subtract-dayTimeDuration-from-time" operator that  :)
(:returns a negative value.                              :)
(:*******************************************************:)
 
(xs:time("01:01:01") - xs:dayTimeDuration("-P11DT02H02M"))