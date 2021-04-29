/******************************************************************************
*                 SOFA, Simulation Open-Framework Architecture                *
*                    (c) 2006 INRIA, USTL, UJF, CNRS, MGH                     *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#pragma once

#include "MyUniformVelocityDampingForceField.h"
#include <sofa/defaulttype/BaseMatrix.h>
#include <sofa/core/MechanicalParams.h>
#include <SofaBaseLinearSolver/FullVector.h>
#include <sofa/core/behavior/ForceField.inl>
#include <sofa/core/behavior/MechanicalState.h>
#include <algorithm>
#include <ctime>
#include <sofa/helper/OptionsGroup.h>
#include <iostream>

namespace sofa::component::forcefield
{


template<class DataTypes>
MyUniformVelocityDampingForceField<DataTypes>::MyUniformVelocityDampingForceField()
    : dampingCoefficient(initData(&dampingCoefficient, Real(0.0), "dampingCoefficient", "velocity damping coefficient"))
    , d_implicit(initData(&d_implicit, false, "implicit", "should it generate damping matrix df/dv? (explicit otherwise, i.e. only generating a force)"))
    , d_indices(initData(&d_indices, "indices", "The list of indices impacted by the velocity damping."))
{
    core::objectmodel::Base::addAlias( &dampingCoefficient, "damping" );
}

template<class DataTypes>
void MyUniformVelocityDampingForceField<DataTypes>::addForce (const core::MechanicalParams*, DataVecDeriv&_f, const DataVecCoord& _dx, const DataVecDeriv&_v)
{
    sofa::helper::WriteAccessor<DataVecDeriv> f(_f);
    const VecDeriv& v = _v.getValue();
    const VecCoord& x = _dx.getValue();
//    std::cout << "the size of _dx : "<< x.size()<< std::endl;
//    std::cout << "the size of v is : "<< v.size()<< std::endl;

    for(unsigned int i=0; i<d_indices.getValue().size(); i++){
        unsigned int index = d_indices.getValue()[i];
        f[index] -= v[index]*dampingCoefficient.getValue();
    }
}

template<class DataTypes>
void MyUniformVelocityDampingForceField<DataTypes>::addDForce(const core::MechanicalParams* mparams, DataVecDeriv& d_df , const DataVecDeriv& d_dx)
{
    (void)mparams->kFactor(); // get rid of warning message

    if( !d_implicit.getValue() ) return;

    Real bFactor = (Real)sofa::core::mechanicalparams::bFactor(mparams);

    if( bFactor )
    {
        sofa::helper::WriteAccessor<DataVecDeriv> df(d_df);
        const VecDeriv& dx = d_dx.getValue();

        bFactor *= dampingCoefficient.getValue();

        for(unsigned int i=0; i<dx.size(); i++)
            df[i] -= dx[i]*bFactor;
    }
}

template<class DataTypes>
void MyUniformVelocityDampingForceField<DataTypes>::addBToMatrix(sofa::defaulttype::BaseMatrix * mat, SReal bFact, unsigned int& offset)
{
    if( !d_implicit.getValue() ) return;

    const unsigned int size = this->mstate->getMatrixSize();

    for( unsigned i=0 ; i<size ; i++ )
        mat->add( offset+i, offset+i, -dampingCoefficient.getValue()*bFact );
}


template<typename DataTypes>
void MyUniformVelocityDampingForceField<DataTypes>::addKToMatrix(const core::MechanicalParams* mparams, const sofa::core::behavior::MultiMatrixAccessor* matrix )
{
    sofa::core::behavior::MultiMatrixAccessor::MatrixRef mref = matrix->getMatrix(this->mstate);

    sofa::defaulttype::BaseMatrix* mat = mref.matrix;
    unsigned int offset = mref.offset;

    Real bFactor = (Real)sofa::core::mechanicalparams::bFactor(mparams);
    const VecCoord& pos = this->mstate->read(core::ConstVecCoordId::position())->getValue();

    const unsigned int size = this->mstate->getMatrixSize();

    for (unsigned int n=0; n<d_indices.getValue().size(); n++){
        unsigned int index = d_indices.getValue()[n];
        mat->add( offset+index, offset+index, -dampingCoefficient.getValue()*bFactor );
    }

//    for (unsigned int n=0; n<d_indices.getValue().size(); n++)
//    {
//        unsigned int index = d_indices.getValue()[n];
//        for(int i = 0; i < 3; i++)
//        {
//            for (int j = 0; j< 3; j++){
//                mat->add(offset + i + 3*index, offset + j + 3*index, -dampingCoefficient.getValue()*bFactor);
//            }
//        }
//    }
}
// template<class DataTypes>
// void MyUniformVelocityDampingForceField<DataTypes>::addKToMatrix(sofa::defaulttype::BaseMatrix * mat, SReal bFact, unsigned int& offset)
// {
//     if( !d_implicit.getValue() ) return;
// 
//     const unsigned int size = this->mstate->getMatrixSize();
// 
//     for( unsigned i=0 ; i<size ; i++ )
//         mat->add( offset+i, offset+i, -dampingCoefficient.getValue()*bFact );
// }

template <class DataTypes>
SReal MyUniformVelocityDampingForceField<DataTypes>::getPotentialEnergy(const core::MechanicalParams*, const DataVecCoord&) const
{
    // TODO
    return 0;
}


} // namespace sofa::component::forcefield