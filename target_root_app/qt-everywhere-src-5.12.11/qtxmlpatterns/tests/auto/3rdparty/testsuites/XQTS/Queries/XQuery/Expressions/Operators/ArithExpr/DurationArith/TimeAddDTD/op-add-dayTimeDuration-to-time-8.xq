(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-time-8                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-time" function that  :)
(:returns a negative value.                              :)
(:*******************************************************:)
 
(xs:time("08:02:06") + xs:dayTimeDuration("-P11DT02H02M"))