import numpy as np

X = np.array([[100.,100.], [10.,10.]])

A = np.array([[0.,1.],[0.,-0.7]])

dt = 1./30.
B = np.array([[80.,0.],[0.,80.]])
Binv = np.linalg.inv(B)
print(Binv)

Xr = np.array([X[0],[0,0]])
Xtmp = (X + (dt * np.dot(A, X))) - Xr
Rtmp = Binv / dt

u = np.dot(-Xtmp, Rtmp)


Xdot = (X + (dt * np.dot(A, X))) + np.dot((dt * B), u)

print(Xtmp)
print(Rtmp)

print(u)
print(Xdot)

# J = (X + (dt * A * X))
# K = (dt * B) # * ??? = J

# print(J)
# print(K)

# j = np.array([[100.,         103.33333333],
#               [ 10.,           9.76666667]])

# k = B


# # k * ???? = j
# uu = np.dot(J, np.linalg.inv(k))

# print("##################")
# print(j)
# print("")
# print(k)
# print("")
# print(np.linalg.inv(k))
# print("")
# # print(np.linalg.inv(k))
# print(uu)
# print("")
# # print(k*uu)
# # print(j)

# # print (j - (k*uu))